-- Haven shared rental database
-- Run this script once in the Supabase SQL Editor.

create extension if not exists pgcrypto;
create schema if not exists private;
revoke all on schema private from public, anon, authenticated;

create table if not exists public.owner_accounts (
  user_id uuid primary key references auth.users(id) on delete cascade,
  created_at timestamptz not null default now()
);
alter table public.owner_accounts enable row level security;
revoke all on public.owner_accounts from public, anon, authenticated;

create table if not exists public.tenant_profiles (
  id uuid primary key references auth.users(id) on delete cascade,
  room_no smallint not null unique check (room_no between 1 and 18),
  full_name text not null check (length(trim(full_name)) between 1 and 100),
  phone text not null check (length(trim(phone)) between 1 and 30),
  address text not null check (length(trim(address)) between 1 and 300),
  created_at timestamptz not null default now(),
  updated_at timestamptz not null default now()
);

create table if not exists public.payment_submissions (
  id uuid primary key default gen_random_uuid(),
  tenant_id uuid not null references public.tenant_profiles(id) on delete cascade,
  payment_month text not null check (payment_month ~ '^\d{4}-(0[1-9]|1[0-2])$'),
  payment_reference text not null default '' check (length(payment_reference) <= 150),
  screenshot_filename text not null check (length(trim(screenshot_filename)) between 1 and 200),
  status text not null default 'pending' check (status in ('pending','approved','rejected')),
  submitted_at timestamptz not null default now(),
  reviewed_at timestamptz
);

create index if not exists payment_submissions_tenant_submitted_idx
  on public.payment_submissions(tenant_id, submitted_at desc);

create or replace function private.touch_updated_at()
returns trigger
language plpgsql
set search_path = ''
as $$
begin
  new.updated_at := now();
  return new;
end;
$$;

drop trigger if exists tenant_profiles_touch_updated_at on public.tenant_profiles;
create trigger tenant_profiles_touch_updated_at
before update on public.tenant_profiles
for each row execute function private.touch_updated_at();

-- Only the matching signed-in user ID can be promoted through the SQL Editor.
-- The function reveals only whether the current authenticated user is an owner.
create or replace function public.is_owner()
returns boolean
language sql
stable
security definer
set search_path = ''
as $$
  select exists (
    select 1 from public.owner_accounts oa
    where oa.user_id = (select auth.uid())
  );
$$;
revoke all on function public.is_owner() from public, anon;
grant execute on function public.is_owner() to authenticated;

-- Returns room numbers, rent amounts, and occupancy only; it never returns tenant data.
create or replace function public.get_room_availability()
returns table(room_no integer, monthly_rent integer, occupied boolean)
language sql
stable
security definer
set search_path = ''
as $$
  select rooms.room_no,
         case when rooms.room_no between 1 and 10 then 5200 else 6200 end,
         exists (
           select 1 from public.tenant_profiles tp
           where tp.room_no = rooms.room_no
         )
  from generate_series(1, 18) as rooms(room_no)
  order by rooms.room_no;
$$;
revoke all on function public.get_room_availability() from public;
grant execute on function public.get_room_availability() to anon, authenticated;

alter table public.tenant_profiles enable row level security;
alter table public.payment_submissions enable row level security;

drop policy if exists tenant_profiles_read_self_or_owner on public.tenant_profiles;
create policy tenant_profiles_read_self_or_owner
  on public.tenant_profiles for select to authenticated
  using ((select auth.uid()) = id or (select public.is_owner()));

drop policy if exists tenant_profiles_insert_self on public.tenant_profiles;
create policy tenant_profiles_insert_self
  on public.tenant_profiles for insert to authenticated
  with check ((select auth.uid()) = id and not (select public.is_owner()));

drop policy if exists tenant_profiles_update_self on public.tenant_profiles;
create policy tenant_profiles_update_self
  on public.tenant_profiles for update to authenticated
  using ((select auth.uid()) = id and not (select public.is_owner()))
  with check ((select auth.uid()) = id and not (select public.is_owner()));

drop policy if exists payments_read_self_or_owner on public.payment_submissions;
create policy payments_read_self_or_owner
  on public.payment_submissions for select to authenticated
  using (tenant_id = (select auth.uid()) or (select public.is_owner()));

drop policy if exists payments_insert_self_pending on public.payment_submissions;
create policy payments_insert_self_pending
  on public.payment_submissions for insert to authenticated
  with check (
    tenant_id = (select auth.uid())
    and status = 'pending'
    and reviewed_at is null
  );

drop policy if exists payments_update_owner on public.payment_submissions;
create policy payments_update_owner
  on public.payment_submissions for update to authenticated
  using ((select public.is_owner()))
  with check ((select public.is_owner()));

grant usage on schema public to anon, authenticated;
grant select, insert on public.tenant_profiles to authenticated;
grant update(full_name, phone, address) on public.tenant_profiles to authenticated;
grant select, insert on public.payment_submissions to authenticated;
grant update(status, reviewed_at) on public.payment_submissions to authenticated;

-- Realtime lets a signed-in owner see tenant edits and new payment proofs immediately.
do $$
begin
  if not exists (
    select 1 from pg_publication_tables
    where pubname = 'supabase_realtime' and schemaname = 'public' and tablename = 'tenant_profiles'
  ) then
    execute 'alter publication supabase_realtime add table public.tenant_profiles';
  end if;
  if not exists (
    select 1 from pg_publication_tables
    where pubname = 'supabase_realtime' and schemaname = 'public' and tablename = 'payment_submissions'
  ) then
    execute 'alter publication supabase_realtime add table public.payment_submissions';
  end if;
end;
$$;
